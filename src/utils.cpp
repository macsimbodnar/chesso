#include "utils.hpp"
#include <bitset>
#include <cassert>
#include <iterator>
#include <sstream>
#include <unordered_map>


bool is_uint(const std::string& str)
{
  for (const char c : str) {
    if (!isdigit(c)) { return false; }
  }

  return true;
}


std::vector<std::string> split_string(const std::string& str)
{
  std::stringstream ss(str);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> tokens(begin, end);

  return tokens;
}


std::string index_to_str(index_t index)
{
  // clang-format off
  static const char* map[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
  };
  // clang-format on

  assert(index < sizeof(map) / sizeof(map[0]));

  return map[index];
}


index_t position_to_index(const uint8_t file, const uint8_t rank)
{
  assert(file >= 0 && file < 8);
  assert(rank >= 0 && rank < 8);

  const index_t index = (7 - rank) * 8 + file;
  assert(index < 64);

  return index;
}


position_t index_to_position(index_t index)
{
  assert(index < 64);

  position_t result;
  result.file = index % 8;
  result.rank = 7 - (index / 8);

  return result;
}


index_t str_to_index(const std::string& p)
{
  assert(p.length() == 2);

  uint8_t file = p[0];
  uint8_t rank = p[1];

  assert(file >= 'a');
  assert(file <= 'h');
  assert(rank >= '1');
  assert(rank <= '8');

  file = file - 'a';
  rank = rank - '1';

  const index_t result = position_to_index(file, rank);

  return result;
}


piece_t char_to_piece(char c)
{
  static const std::unordered_map<char, piece_t> char_to_piece_map = {
      {'p', B_PAWN},   {'n', B_KNIGHT}, {'b', B_BISHOP}, {'r', B_ROOK},
      {'q', B_QUEEN},  {'k', B_KING},   {'P', W_PAWN},   {'N', W_KNIGHT},
      {'B', W_BISHOP}, {'R', W_ROOK},   {'Q', W_QUEEN},  {'K', W_KING}};

  return char_to_piece_map.at(c);
}


inline std::string piece_to_icon(piece_t piece)
{
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
    {B_KING, "♔"}
  };
  // clang-format on

  return sprite_map.at(piece);
}


inline char piece_to_char(piece_t piece)
{
  const static char ascii_pieces[] = "PNBRQKpnbrqk";
  return ascii_pieces[piece];
}


std::string piece_to_str(piece_t piece)
{
  std::string res;

  res += piece_to_icon(piece);
  // res += piece_to_char(piece);

  return res;
}


std::string promotion_to_str(promotion_t piece)
{
  static const std::string value[] = {"", "n", "b", "r", "q"};
  return value[piece];
}


std::string print_bboard(bb_t board)
{
  std::stringstream ss;
  ss << "\n";

  ss << "    bitboard: " << std::hex << board << std::dec << "\n\n";

  for (int r_index = 0; r_index < 8; ++r_index) {
    const int rank = 7 - r_index;

    for (int file = 0; file < 8; ++file) {
      const int square = position_to_index(file, rank);

      // Print ranks
      if (file == 0) { ss << rank + 1 << "   "; }

      // Print the bit
      ss << (GET_BIT(board, square) ? 1 : 0) << " ";
    }

    ss << "\n";
  }

  // Print the files
  ss << "\n    A B C D E F G H \n";
  return ss.str();
}


std::string print_nice_board(const board_t* board)
{
  assert(board != nullptr);

  std::stringstream ss;

  /**
   * 8  ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
   * 7  ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟
   * 6
   * 5
   * 4
   * 3
   * 2  ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙
   * 1  ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖
   *    A  B  C  D  E  F  G  H
   */

  ss << "#######################################\n";

  for (uint8_t r_index = 0; r_index < 8; ++r_index) {
    const uint8_t rank = 7 - r_index;
    ss << int(rank + 1) << "  ";

    for (uint8_t file = 0; file < 8; ++file) {
      const index_t square = position_to_index(file, rank);

      // loop over all piece bitboards
      bool empty_square = true;
      for (int piece = W_PAWN; piece <= B_KING; ++piece) {
        if (GET_BIT(board->bitboards[piece], square)) {
          // ss << piece_to_str(static_cast<piece_t>(piece)) << " ";
          ss << piece_to_str(static_cast<piece_t>(piece)) << " ";
          empty_square = false;
          break;
          ;
        }
      }

      if (empty_square) {
        if ((file + rank) % 2) {
          // WHITE EMPTY SQUARE
          ss << "  ";
        } else {
          // BLACK EMPTY SQUARE
          ss << "* ";
        }
      }
    }

    ss << "\n";
  }

  // clang-format off
  ss << "   A B C D E F G H";
  ss << "\n------------------";

  ss << "\nactive_color:      " << ((board->active_color == WHITE) ? "WHITE" : "BLACK");
  ss << "\ncastling:          " << std::bitset<4>(board->castling);
  ss << "\nhalf_move_clock:   " << int(board->halfmove_clock);
  ss << "\nen_passant:        " << ((board->en_passant == INVALID_INDEX) ? "-" : index_to_str(board->en_passant));
  ss << "\nfull_move_number:  " << int(board->fullmove_counter);
  ss << "\nhash:              " << board->hash;
  // ss << "\nphase_value:       " << int(board->phase_value);
  // ss << "next_move:         " << board->next_move;

  ss << "\n#######################################";
  // clang-format on

  return ss.str();
}


std::string print_move(move_t move)
{
  std::stringstream ss;
  unpacked_move_t m(move);

  ss << index_to_str(m.from) << index_to_str(m.to);
  ss << " " << piece_to_str(m.piece);

  if (m.promoted_to > TO_NONE) { ss << " " << promotion_to_str(m.promoted_to); }
  if (m.capture) { ss << " capture"; }
  if (m.double_push) { ss << " double push"; }
  if (m.en_passant) { ss << " en passant"; }
  if (m.castling) { ss << " castling"; }

  return ss.str();
}
