#pragma once
#include <array>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <map>
#include <sstream>
#include <vector>


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

#ifndef STR
#define STR(_N_) std::to_string(_N_)
#endif

#define BOARD_SIZE 128
#define DEFAULT_POSITION \
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define B_PAWN 'p'
#define B_KNIGHT 'n'
#define B_BISHOP 'b'
#define B_ROOK 'r'
#define B_QUEEN 'q'
#define B_KING 'k'
#define W_PAWN 'P'
#define W_KNIGHT 'N'
#define W_BISHOP 'B'
#define W_ROOK 'R'
#define W_QUEEN 'Q'
#define W_KING 'K'
#define INVALID '*'
#define EMPTY ' '

static constexpr uint8_t INVALID_BOARD_INDEX = 127;

// Castling rights
static constexpr uint8_t WQ = 0b0000001;
static constexpr uint8_t WK = 0b0000010;
static constexpr uint8_t BQ = 0b0000100;
static constexpr uint8_t BK = 0b0001000;

typedef char piece_t;

struct position_t
{
  uint8_t file;
  uint8_t rank;
};


enum class color_t
{
  WHITE,
  BLACK
};


class FAN_exception : public std::runtime_error
{
public:
  FAN_exception(std::string msg) : std::runtime_error(std::move(msg)) {}
};


inline uint8_t position_to_index(const uint8_t file, const uint8_t rank)
{
  assert(file >= 0 && file < 8);
  assert(rank >= 0 && rank < 8);

  const uint8_t index = (rank << 4) + file;
  assert(index < BOARD_SIZE);
  assert(!(index & 0x88));

  return index;
}


inline position_t index_to_position(uint8_t index)
{
  assert(!(index & 0x88));
  assert(index < BOARD_SIZE);

  position_t result;
  result.file = index & 7;
  result.rank = index >> 4;

  return result;
}


inline uint8_t algebraic_to_index(const std::string& p)
{
  uint8_t result = INVALID_BOARD_INDEX;

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


inline std::string index_to_algebraic(const uint8_t i)
{
  if (i == INVALID_BOARD_INDEX) { return "-"; }

  const position_t p = index_to_position(i);
  std::string result;
  result.reserve(2);

  result.push_back('a' + p.file);
  result.push_back('1' + p.rank);

  return result;
}


inline bool is_uint(const std::string& str)
{
  for (const char c : str) {
    if (!isdigit(c)) { return false; }
  }

  return true;
}


inline std::vector<std::string> split_string(const std::string& str)
{
  std::stringstream ss(str);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> tokens(begin, end);

  return tokens;
}


class board_t
{
private:
  std::array<piece_t, BOARD_SIZE> board = {EMPTY};
  color_t turn = color_t::WHITE;
  bool is_en_passant = false;
  uint8_t en_passant_target_square;
  int half_moves = 0;
  int move_numbers = 0;
  uint8_t available_castling = 0x00 | (WQ | WK | BQ | BK);

public:
  std::string print_board()
  {
    std::stringstream ss;

    for (size_t i = 8; i > 0; --i) {
      for (size_t j = 0; j < 16; ++j) {
        ss << board[((i - 1) * 16) + j];
      }

      ss << "\n";
    }

    return ss.str();
  }

  std::string print_nice_board()
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
    static const std::map<char, std::string> sprite_map = {
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

    for (size_t i = 8; i > 0; --i) {
      ss << i << "  ";

      for (size_t j = 0; j < 16; ++j) {
        const uint8_t index = ((i - 1) * 16) + j;
        const char piece = board[index];

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

    ss << "   A B C D E F G H";

    return ss.str();
  }


  void cleanup()
  {
    for (uint32_t i = 0; i < board.size(); ++i) {
      board[i] = (i & 0x88) ? INVALID : EMPTY;
    }

    turn = color_t::WHITE;
    is_en_passant = false;
    en_passant_target_square = INVALID_BOARD_INDEX;
    half_moves = 0;
    move_numbers = 0;
    available_castling = 0x00 | (WQ | WK | BQ | BK);
  }

  void load_FEN(const std::string& FEN)
  {
    // Clean the board first
    cleanup();

    bool b_king_set = false;
    bool w_king_set = false;

    // Start parsing
    auto sections = split_string(FEN);

    if (sections.size() != 6) { throw FAN_exception("Bad FEN string: " + FEN); }

    /*****************************************************************************
     * 0. Piece placement data
     *
     * pawn = "P"
     * knight = "N"
     * bishop = "B"
     * rook = "R"
     * queen = "Q"
     * and king = "K
     *
     * White ("PNBRQK")
     * Black ("pnbrqk")
     ****************************************************************************/

    uint8_t file = 0;
    uint8_t rank = 7;

    for (const char c : sections[0]) {
      switch (c) {
        case '/':
          file = 0;
          --rank;
          break;

        case '1':
          file += 1;
          break;
        case '2':
          file += 2;
          break;
        case '3':
          file += 3;
          break;
        case '4':
          file += 4;
          break;
        case '5':
          file += 5;
          break;
        case '6':
          file += 6;
          break;
        case '7':
          file += 7;
          break;
        case '8':
          file += 8;
          break;

        case 'P':
        case 'N':
        case 'B':
        case 'R':
        case 'Q':
        case 'K':
        case 'p':
        case 'n':
        case 'b':
        case 'r':
        case 'q':
        case 'k': {
          const uint8_t index = position_to_index(file, rank);
          board[index] = c;
          ++file;
        } break;

        default:
          // We get a non valid string
          throw FAN_exception("Invalid char in FEN string [" + STR(c) +
                              "]. FEN: " + FEN);
          break;
      }
    }

    /***************************************************************************
     * 1. Active color
     **************************************************************************/
    if (sections[1].size() != 1) {
      throw FAN_exception("invalid active color section. FEN: " + FEN);
    }

    char color = sections[1][0];
    switch (color) {
      case 'w':
        turn = color_t::WHITE;
        break;
      case 'b':
        turn = color_t::BLACK;
        break;
      default:
        throw FAN_exception("Invalid color char in FEN string [" +
                            std::string(1, color) + "]. FEN: " + FEN);
    }

    /***************************************************************************
     * 2. Castling availability
     *
     * "-" No castling available
     * "K" if White can castle kingside
     * "Q" if White can castle queenside
     * "k" if Black can castle kingside
     * "q" if Black can castle queenside
     **************************************************************************/
    if (sections[2].size() < 1 || sections[2].size() > 4) {
      throw FAN_exception("Invalid castling availability section size. FEN: " +
                          FEN);
    }

    available_castling = 0x00;
    for (const char c : sections[2]) {
      switch (c) {
        case '-':
          available_castling = 0x00;
          if (sections[2].size() != 1) {
            throw FAN_exception(
                "Invalid castling availability section size. No castling "
                "available char is set but the section size is too big. FEN: " +
                FEN);
          }
          break;
        case 'K':
          available_castling |= WK;
          break;
        case 'Q':
          available_castling |= WQ;
          break;
        case 'k':
          available_castling |= BK;
          break;
        case 'q':
          available_castling |= BQ;
          break;

        default:
          throw FAN_exception("Invalid castling availability character [" +
                              std::string(1, c) + "]. FEN: " + FEN);
      }
    }

    /***************************************************************************
     * 3. En passant target square
     *
     * "-" None
     **************************************************************************/
    if (sections[3].size() < 1 || sections[3].size() > 2) {
      throw FAN_exception("Invalid en passant section size. FEN: " + FEN);
    }

    if (sections[3].size() == 1 && sections[3][0] != '-') {
      throw FAN_exception("Invalid en passant section char [ " +
                          std::string(1, sections[3][0]) + "]. FEN: " + FEN);
    }

    if (sections[3].size() == 2) {
      if (!isalpha(sections[3][0]) || !isdigit(sections[3][1])) {
        throw FAN_exception(
            "Invalid en passant section. Wrong algebraic notation [" +
            sections[3] + "]. FEN: " + FEN);
      }
    }


    en_passant_target_square = algebraic_to_index(sections[3]);

    /***************************************************************************
     * 4. Halfmove clock
     *
     * The number of halfmoves since the last capture or pawn advance, used for
     * the fifty-move rule.
     **************************************************************************/
    const std::string& half_move = sections[4];
    if (half_move.size() < 1) {
      throw FAN_exception("Invalid Halfmove clock section size. FEN: " + FEN);
    }

    if (!is_uint(half_move)) {
      throw FAN_exception(
          "Invalid Halfmove clock section is not a number. FEN: " + FEN);
    }

    try {
      half_moves = static_cast<int>(std::stoul(half_move));
    } catch (std::exception& e) {
      throw FAN_exception("Can't convert Halfmove clock to integer.What: " +
                          std::string(e.what()) + " FEN: " + FEN);
    }

    /***************************************************************************
     * 5. Fullmove number
     *
     * The number of the full moves. It starts at 1 and is incremented after
     * Black's move.
     **************************************************************************/
    const std::string& full_move = sections[5];
    if (full_move.size() < 1) {
      throw FAN_exception("Invalid Fullmove number section size. FEN: " + FEN);
    }

    if (!is_uint(full_move)) {
      throw FAN_exception(
          "Invalid Fullmove number section is not a number. FEN: " + FEN);
    }

    try {
      move_numbers = static_cast<int>(std::stoul(full_move));
    } catch (std::exception& e) {
      throw FAN_exception("Can't convert Fullmove number to integer. What: " +
                          std::string(e.what()) + " FEN: " + FEN);
    }

    if (move_numbers < 1) {
      throw FAN_exception("Fullmove number can't be less then 1 but it is " +
                          std::string(STR(move_numbers)) + " FEN: " + FEN);
    }

    // TODO(Max): deal with this
    /***************************************************************************
     * Update headers
    //
    **************************************************************************/
    // if (FEN != std::string(DEFAULT_POSITION)) {
    //   header["SetUp"] = "1";
    //   header["FEN"] = FEN;
    // }

    /***************************************************************************
     * Set repetition
     **************************************************************************/
    // const std::string short_fen = generate_FEN(true);
    // repetition_position_count[short_fen] = 1;

    /***************************************************************************
     * Sanity check with FEN generation
     **************************************************************************/
    const std::string full_FEN = generate_FEN();
    assert(FEN == full_FEN);
  }


  std::string generate_FEN(bool generate_trimmed = false)
  {
    std::stringstream ss;

    // Step 1: Board representation
    int empty_count = 0;

    for (int rank = 7; rank >= 0; --rank) {
      for (int file = 0; file < 8; ++file) {
        const uint8_t index = position_to_index(file, rank);
        const auto piece = board[index];

        if (piece == EMPTY) {
          ++empty_count;
        } else {
          if (empty_count > 0) {
            ss << empty_count;
            empty_count = 0;
          }

          switch (piece) {
            case B_KING:
              ss << 'k';
              break;
            case B_QUEEN:
              ss << 'q';
              break;
            case B_ROOK:
              ss << 'r';
              break;
            case B_BISHOP:
              ss << 'b';
              break;
            case B_KNIGHT:
              ss << 'n';
              break;
            case B_PAWN:
              ss << 'p';
              break;
            case W_KING:
              ss << 'K';
              break;
            case W_QUEEN:
              ss << 'Q';
              break;
            case W_ROOK:
              ss << 'R';
              break;
            case W_BISHOP:
              ss << 'B';
              break;
            case W_KNIGHT:
              ss << 'N';
              break;
            case W_PAWN:
              ss << 'P';
              break;

            default:
              break;
          }
        }
      }

      if (empty_count > 0) {
        ss << empty_count;
        empty_count = 0;
      }

      if (rank > 0) { ss << '/'; }
    }

    // Step 2: Active color
    ss << (turn == color_t::WHITE ? " w " : " b ");

    // Step 3: Castling rights
    bool has_castling_rights = false;

    if (available_castling & WK) {
      ss << 'K';
      has_castling_rights = true;
    }

    if (available_castling & WQ) {
      ss << 'Q';
      has_castling_rights = true;
    }

    if (available_castling & BK) {
      ss << 'k';
      has_castling_rights = true;
    }

    if (available_castling & BQ) {
      ss << 'q';
      has_castling_rights = true;
    }

    if (!has_castling_rights) { ss << '-'; }

    ss << ' ';

    // Step 4: En passant target square
    if (en_passant_target_square != INVALID) {
      ss << index_to_algebraic(en_passant_target_square);
    } else {
      ss << '-';
    }

    ss << ' ';

    if (!generate_trimmed) {
      // Step 5: Halfmove clock
      ss << half_moves << ' ';

      // Step 6: Fullmove number
      ss << move_numbers;
    }

    return ss.str();
  }
};