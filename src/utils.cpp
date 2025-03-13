#include "utils.hpp"
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <unordered_map>
#include "exceptions.hpp"


index_t position_to_index(const uint8_t file, const uint8_t rank)
{
  assert(file >= 0 && file < 8);
  assert(rank >= 0 && rank < 8);

  const index_t index = (rank << 4) + file;
  assert(index < BOARD_SIZE);
  assert(!(index & 0x88));

  return index;
}


position_t index_to_position(index_t index)
{
  assert(!(index & 0x88));
  assert(index < BOARD_SIZE);

  position_t result;
  result.file = index & 7;
  result.rank = index >> 4;

  return result;
}


index_t string_coordinates_to_index(const std::string& p)
{
  index_t result = INVALID_BOARD_INDEX;

  if (p.size() != 2) { return INVALID_BOARD_INDEX; }

  uint8_t file = p[0];
  uint8_t rank = p[1];

  assert(file >= 'a');
  assert(file <= 'h');
  assert(rank >= '1');
  assert(rank <= '8');

  file = file - 'a';
  rank = rank - '1';

  result = position_to_index(file, rank);

  return result;
}


std::string index_to_string_coordinates(const index_t i)
{
  if (i == INVALID_BOARD_INDEX) { return "-"; }

  const position_t p = index_to_position(i);
  std::string result;
  result.reserve(2);

  result.push_back('a' + p.file);
  result.push_back('1' + p.rank);

  return result;
}


char piece_to_char(const piece_t piece)
{
  static const std::unordered_map<piece_t, char> piece_to_char_map = {
      {B_PAWN, 'p'},   {B_KNIGHT, 'n'}, {B_BISHOP, 'b'}, {B_ROOK, 'r'},
      {B_QUEEN, 'q'},  {B_KING, 'k'},   {W_PAWN, 'P'},   {W_KNIGHT, 'N'},
      {W_BISHOP, 'B'}, {W_ROOK, 'R'},   {W_QUEEN, 'Q'},  {W_KING, 'K'},
      {INVALID, '*'},  {EMPTY, ' '}};

  return piece_to_char_map.at(piece);
}


piece_t char_to_piece(const char c)
{
  static const std::unordered_map<char, piece_t> char_to_piece_map = {
      {'p', B_PAWN},   {'n', B_KNIGHT}, {'b', B_BISHOP}, {'r', B_ROOK},
      {'q', B_QUEEN},  {'k', B_KING},   {'P', W_PAWN},   {'N', W_KNIGHT},
      {'B', W_BISHOP}, {'R', W_ROOK},   {'Q', W_QUEEN},  {'K', W_KING},
      {'*', INVALID},  {' ', EMPTY}};

  return char_to_piece_map.at(c);
}


std::string print_board(const board_t* board)
{
  std::stringstream ss;

  for (size_t i = 8; i > 0; --i) {
    for (size_t j = 0; j < 16; ++j) {
      ss << piece_to_char(board->board[((i - 1) * 16) + j]);
    }

    ss << "\n";
  }

  return ss.str();
}

std::string print_nice_board(const board_t* board)
{
  /**
   *
   * 8  ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
   * 7  ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟
   * 6
   * 5
   * 4
   * 3
   * 2  ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙
   * 1  ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖
   *
   *    A B C D E F G H
   */

  // clang-format off
  static const std::unordered_map<char, std::string> sprite_map = {
    {W_PAWN, "♟︎"},
    {W_KNIGHT, "♞"},
    {W_BISHOP, "♝"},
    {W_ROOK, "♜"},
    {W_QUEEN, "♛"},
    {W_KING, "♚"},
    {B_PAWN, "♙"},
    {B_KNIGHT, "♘"},
    {B_BISHOP, "♗"},
    {B_ROOK, "♖"},
    {B_QUEEN, "♕"},
    {B_KING, "♔"},
    {EMPTY, " "},
    {INVALID, " "},
  };
  // clang-format on

  std::stringstream ss;

  ss << "##################\n";

  for (size_t i = 8; i > 0; --i) {
    ss << i << "  ";

    for (size_t j = 0; j < 16; ++j) {
      const index_t index = ((i - 1) * 16) + j;
      const char piece = board->board[index];

      if (index & 0x88) { continue; }

      if (piece == EMPTY) {
        const uint8_t file = index & 7;
        const uint8_t rank = index >> 4;


        if ((file + rank) % 2) {
          // WHITE EMPTY SQUARE
          ss << "  ";
        } else {
          // BLACK EMPTY SQUARE
          ss << "* ";
        }
      } else {
        ss << sprite_map.at(piece) << " ";
      }


      // if (file + rank) % 2 == 0:
      //         square.color = Piece.PieceColor.WHITE
      //     else:
      //         square.color = Piece.PieceColor.BLACK
    }

    ss << "\n";
  }

  // clang-format off
  ss << "   A B C D E F G H";
  ss << "\n------------------";

  ss << "\nactive_color:      " << color_to_string(board->game_state.active_color);
  ss << "\ncastling:          " << std::bitset<4>(board->game_state.castling);
  ss << "\nhalf_move_clock:   " << int(board->game_state.halfmove_clock);
  ss << "\nen_passant:        " << index_to_string_coordinates(board->game_state.en_passant);
  ss << "\nfull_move_number:  " << int(board->game_state.fullmove_counter);
  ss << "\nzobrist_key:       " << board->game_state.zobrist_key;
  // ss << "\nphase_value:       " << int(board->game_state.phase_value);
  // ss << "next_move:         " << board->game_state.next_move;

  ss << "\n##################";
  // clang-format on

  return ss.str();
}


std::string color_to_string(color_t color)
{
  if (color == WHITE) { return "White"; }

  return "Black";
}


color_t get_square_color(index_t index)
{
  position_t pos = index_to_position(index);

  if (((pos.file + pos.rank) % 2) == 0) { return BLACK; }

  return WHITE;
}


color_t get_piece_color(piece_t piece)
{
  assert(piece != EMPTY && piece != INVALID);

  switch (piece) {
    case B_KING:
    case B_QUEEN:
    case B_KNIGHT:
    case B_BISHOP:
    case B_ROOK:
    case B_PAWN:
      return BLACK;
    case W_KING:
    case W_QUEEN:
    case W_KNIGHT:
    case W_BISHOP:
    case W_ROOK:
    case W_PAWN:
      return WHITE;
    case EMPTY:
    case INVALID:
    default:
      assert(false);
      break;
  }

  assert(false);
  return BLACK;
}
