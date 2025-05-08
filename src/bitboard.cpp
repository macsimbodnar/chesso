#include "bitboard.hpp"
#include <bit>
#include "bb_tables.hpp"
#include "data_structures.hpp"
#include "log.hpp"
#include "utils.hpp"


/******************************************************************************
 *                               MUST RUN FAST
 ******************************************************************************/
inline int count_bits(bb_t board)
{
  return std::popcount(board);
}


inline index_t get_lsb_index(bb_t board)
{
  // If 64 then invalid
  return std::countr_zero(board);
}


bb_t get_bishop_attacks(const bb_tables_t* data, index_t index, bb_t occupancy)
{
  assert(data != nullptr);

  occupancy &= data->bishop_masks[index];
  occupancy *= bishop_magic_numbers[index];
  occupancy >>= 64 - bishop_relevant_bits_count[index];
  return data->bishop_attacks[index][occupancy];
}


bb_t get_rook_attacks(const bb_tables_t* data, index_t index, bb_t occupancy)
{
  assert(data != nullptr);

  occupancy &= data->rook_masks[index];
  occupancy *= rook_magic_numbers[index];
  occupancy >>= 64 - rook_relevant_bits_count[index];
  return data->rook_attacks[index][occupancy];
}


bb_t get_queen_attacks(const bb_tables_t* data, index_t index, bb_t occupancy)
{
  bb_t queen_attacks = get_bishop_attacks(data, index, occupancy);
  queen_attacks |= get_rook_attacks(data, index, occupancy);
  return queen_attacks;
}


bool is_attacked(const bb_tables_t* data,
                 const board_t* board,
                 index_t index,
                 color_t color)
{
  assert(board != nullptr);

  {  // Handle pawn attacks
    if ((color == WHITE) &&
        (data->pawn_attacks[BLACK][index] & board->bitboards[W_PAWN])) {
      return true;
    }

    if ((color == BLACK) &&
        (data->pawn_attacks[WHITE][index] & board->bitboards[B_PAWN])) {
      return true;
      ;
    }
  }

  if (data->knight_attacks[index] &
      ((color == WHITE) ? board->bitboards[W_KNIGHT]
                        : board->bitboards[B_KNIGHT])) {
    return true;
  }


  if (get_bishop_attacks(data, index, board->occupancies[BOTH]) &
      ((color == WHITE) ? board->bitboards[W_BISHOP]
                        : board->bitboards[B_BISHOP])) {
    return true;
  }

  if (get_rook_attacks(data, index, board->occupancies[BOTH]) &
      ((color == WHITE) ? board->bitboards[W_ROOK]
                        : board->bitboards[B_ROOK])) {
    return true;
  }

  // attacked by bishops
  if (get_queen_attacks(data, index, board->occupancies[BOTH]) &
      ((color == WHITE) ? board->bitboards[W_QUEEN]
                        : board->bitboards[B_QUEEN])) {
    return true;
  }

  // attacked by kings
  if (data->king_attacks[index] &
      ((color == WHITE) ? board->bitboards[W_KING]
                        : board->bitboards[B_KING])) {
    return true;
  }

  return false;
}


/******************************************************************************
 *                               UTIL FUNCTIONS
 * NOTE: Does not need to be optimized
 ******************************************************************************/
bool load_FEN(const std::string& FEN, board_t* board)
{
  assert(board != nullptr);
  cleanup_board(board);

  // Start parsing
  auto sections = split_string(FEN);

  if (sections.size() != 6) {
    LOG_E << "Bad FEN string: " << FEN << END_E;
    return false;
  }

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
        const index_t index = position_to_index(file, rank);
        const piece_t piece = char_to_piece(c);
        SET_BIT(board->bitboards[piece], index);
        ++file;
      } break;

      default:
        // We get a non valid string
        LOG_E << "Invalid char in FEN string [" << STR(c) << "]. FEN: " << FEN
              << END_E;

        return false;
        break;
    }
  }

  /***************************************************************************
   * 1. Active color
   **************************************************************************/
  if (sections[1].size() != 1) {
    LOG_E << "invalid active color section. FEN: " << FEN << END_E;
    return false;
  }

  char color = sections[1][0];
  switch (color) {
    case 'w':
      board->active_color = WHITE;
      break;
    case 'b':
      board->active_color = BLACK;
      break;
    default:
      LOG_E << "Invalid color char in FEN string [" << std::string(1, color)
            << "]. FEN: " << FEN << END_E;
      return false;
      break;
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
    LOG_E << "Invalid castling availability section size. FEN: " << FEN
          << END_E;
    return false;
  }

  board->castling = 0x00;
  for (const char c : sections[2]) {
    switch (c) {
      case '-':
        board->castling = 0x00;
        if (sections[2].size() != 1) {
          LOG_E
              << "Invalid castling availability section size. No castling "
                 "available char is set but the section size is too big. FEN: "
              << FEN << END_E;
          return false;
        }
        break;
      case 'K':
        board->castling |= WK;
        break;
      case 'Q':
        board->castling |= WQ;
        break;
      case 'k':
        board->castling |= BK;
        break;
      case 'q':
        board->castling |= BQ;
        break;

      default:
        LOG_E << "Invalid castling availability character ["
              << std::string(1, c) << "]. FEN: " + FEN << END_E;

        return false;
    }
  }

  /***************************************************************************
   * 3. En passant target square
   *
   * "-" None
   **************************************************************************/
  if (sections[3].size() < 1 || sections[3].size() > 2) {
    LOG_E << "Invalid en passant section size. FEN: " << FEN << END_E;
    return false;
  }

  if (sections[3].size() == 1 && sections[3][0] != '-') {
    LOG_E << "Invalid en passant section char [ "
          << std::string(1, sections[3][0]) << "]. FEN: " << FEN << END_E;
    return false;
  }

  if (sections[3].size() == 2) {
    if (!isalpha(sections[3][0]) || !isdigit(sections[3][1])) {
      LOG_E << "Invalid en passant section. Wrong algebraic notation ["
            << sections[3] << "]. FEN: " << FEN << END_E;
      return false;
    }
  }

  if (sections[3] == "-") {
    board->en_passant = INVALID_INDEX;
  } else {
    board->en_passant = str_to_index(sections[3]);
  }

  /***************************************************************************
   * 4. Halfmove clock
   *
   * The number of halfmoves since the last capture or pawn advance, used for
   * the fifty-move rule.
   **************************************************************************/
  const std::string& half_move = sections[4];
  if (half_move.size() < 1) {
    LOG_E << "Invalid Halfmove clock section size. FEN: " << FEN << END_E;
    return false;
  }

  if (!is_uint(half_move)) {
    LOG_E << "Invalid Halfmove clock section is not a number. FEN: " << FEN
          << END_E;
    return false;
  }

  try {
    board->halfmove_clock = static_cast<int>(std::stoul(half_move));
  } catch (std::exception& e) {
    LOG_E << "Can't convert Halfmove clock to integer.What: "
          << std::string(e.what()) << " FEN: " << FEN << END_E;
    return false;
  }

  /***************************************************************************
   * 5. Fullmove number
   *
   * The number of the full moves. It starts at 1 and is incremented after
   * Black's move.
   **************************************************************************/
  const std::string& full_move = sections[5];
  if (full_move.size() < 1) {
    LOG_E << "Invalid Fullmove number section size. FEN: " << FEN << END_E;
    return false;
  }

  if (!is_uint(full_move)) {
    LOG_E << "Invalid Fullmove number section is not a number. FEN: " << FEN
          << END_E;
    return false;
  }

  try {
    board->fullmove_counter = static_cast<int>(std::stoul(full_move));
  } catch (std::exception& e) {
    LOG_E << "Can't convert Fullmove number to integer. What: "
          << std::string(e.what()) << " FEN: " << FEN << END_E;
    return false;
  }

  if (board->fullmove_counter < 1) {
    LOG_E << "Fullmove number can't be less then 1 but it is "
          << std::string(STR(board->fullmove_counter)) << " FEN: " << FEN
          << END_E;
    return false;
  }

  // Populate occupancies
  for (int piece = W_PAWN; piece <= W_KING; ++piece) {
    board->occupancies[WHITE] |= board->bitboards[piece];
    board->occupancies[BLACK] |= board->bitboards[piece + B_PAWN];
  }

  board->occupancies[BOTH] |= board->occupancies[WHITE];
  board->occupancies[BOTH] |= board->occupancies[BLACK];

  // Update Zobrist keys
  // board->zobrist_key = init_zobrist_key(state, board);
  return true;
}


std::string generate_FEN(const board_t* board)
{
  assert(board != nullptr);

  std::stringstream ss;

  // Step 1: Board representation
  int empty_count = 0;

  for (int rank = 7; rank >= 0; --rank) {
    for (int file = 0; file < 8; ++file) {
      const auto index = position_to_index(file, rank);
      const auto piece = get_piece(board, index);

      if (piece == piece_t::EMPTY) {
        ++empty_count;
      } else {
        if (empty_count > 0) {
          ss << empty_count;
          empty_count = 0;
        }

        switch (piece) {
          case piece_t::B_KING:
            ss << 'k';
            break;
          case piece_t::B_QUEEN:
            ss << 'q';
            break;
          case piece_t::B_ROOK:
            ss << 'r';
            break;
          case piece_t::B_BISHOP:
            ss << 'b';
            break;
          case piece_t::B_KNIGHT:
            ss << 'n';
            break;
          case piece_t::B_PAWN:
            ss << 'p';
            break;
          case piece_t::W_KING:
            ss << 'K';
            break;
          case piece_t::W_QUEEN:
            ss << 'Q';
            break;
          case piece_t::W_ROOK:
            ss << 'R';
            break;
          case piece_t::W_BISHOP:
            ss << 'B';
            break;
          case piece_t::W_KNIGHT:
            ss << 'N';
            break;
          case piece_t::W_PAWN:
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
  ss << (board->active_color == color_t::WHITE ? " w " : " b ");

  // Step 3: Castling rights
  bool has_castling_rights = false;

  if (board->castling & WK) {
    ss << 'K';
    has_castling_rights = true;
  }

  if (board->castling & WQ) {
    ss << 'Q';
    has_castling_rights = true;
  }

  if (board->castling & BK) {
    ss << 'k';
    has_castling_rights = true;
  }

  if (board->castling & BQ) {
    ss << 'q';
    has_castling_rights = true;
  }

  if (!has_castling_rights) { ss << '-'; }

  ss << ' ';

  // Step 4: En passant target square
  if (board->en_passant != INVALID_INDEX) {
    ss << index_to_str(board->en_passant);
  } else {
    ss << '-';
  }

  ss << ' ';

  // Step 5: Halfmove clock
  ss << int(board->halfmove_clock) << ' ';

  // Step 6: Fullmove number
  ss << int(board->fullmove_counter);

  return ss.str();
}


/******************************************************************************
 *                      INITIALIZATION FUNCTIONS
 * NOTE: Does not need to be optimized, they are called once at the start
 ******************************************************************************/
bb_t set_occupancy(index_t index, int mask_bit_count, bb_t attack_mask)
{
  bb_t occupancy = BB_0;

  for (int count = 0; count < mask_bit_count; ++count) {
    const index_t square = get_lsb_index(attack_mask);
    assert(square < 64);

    POP_BIT(attack_mask, square);

    // Check if on board
    if (index & (BB_1 << count)) { occupancy |= BB_1 << square; }
  }

  return occupancy;
}


bb_t precompute_pawn_attacks(color_t color, index_t square)
{
  assert(square < 64);

  bb_t attacks = BB_0;
  bb_t board = BB_0;
  SET_BIT(board, square);

  switch (color) {
    case WHITE:
      if ((board >> 7) & NOT_A_FILE) { attacks |= (board >> 7); }
      if ((board >> 9) & NOT_H_FILE) { attacks |= (board >> 9); }
      break;

    case BLACK:
      if ((board << 7) & NOT_H_FILE) { attacks |= (board << 7); }
      if ((board << 9) & NOT_A_FILE) { attacks |= (board << 9); }
      break;

    default:
      assert(false);
  }

  return attacks;
}


bb_t precompute_knight_attacks(index_t square)
{
  bb_t attacks = BB_0;
  bb_t board = BB_0;
  SET_BIT(board, square);

  if ((board >> 17) & NOT_H_FILE) { attacks |= (board >> 17); }
  if ((board >> 15) & NOT_A_FILE) { attacks |= (board >> 15); }
  if ((board >> 10) & NOT_GH_FILES) { attacks |= (board >> 10); }
  if ((board >> 6) & NOT_AB_FILES) { attacks |= (board >> 6); }

  if ((board << 17) & NOT_A_FILE) { attacks |= (board << 17); }
  if ((board << 15) & NOT_H_FILE) { attacks |= (board << 15); }
  if ((board << 10) & NOT_AB_FILES) { attacks |= (board << 10); }
  if ((board << 6) & NOT_GH_FILES) { attacks |= (board << 6); }

  return attacks;
}


bb_t precompute_king_attacks(index_t square)
{
  bb_t attacks = BB_0;
  bb_t board = BB_0;
  SET_BIT(board, square);

  if (board >> 8) { attacks |= (board >> 8); }
  if ((board >> 9) & NOT_H_FILE) { attacks |= (board >> 9); }
  if ((board >> 7) & NOT_A_FILE) { attacks |= (board >> 7); }
  if ((board >> 1) & NOT_H_FILE) { attacks |= (board >> 1); }

  if (board << 8) { attacks |= (board << 8); }
  if ((board << 9) & NOT_A_FILE) { attacks |= (board << 9); }
  if ((board << 7) & NOT_H_FILE) { attacks |= (board << 7); }
  if ((board << 1) & NOT_A_FILE) { attacks |= (board << 1); }

  return attacks;
}


bb_t precompute_bishop_attack_masks(index_t square)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1, f = tf + 1; r < 7 && f < 7; ++r, ++f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }
  for (r = tr - 1, f = tf + 1; r > 0 && f < 7; --r, ++f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }
  for (r = tr + 1, f = tf - 1; r < 7 && f > 0; ++r, --f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }
  for (r = tr - 1, f = tf - 1; r > 0 && f > 0; --r, --f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }

  return attacks;
}


bb_t precompute_rook_attack_masks(index_t square)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1; r < 7; ++r) {
    attacks |= (BB_1 << ((r * 8) + tf));
  }
  for (r = tr - 1; r > 0; --r) {
    attacks |= (BB_1 << ((r * 8) + tf));
  }
  for (f = tf + 1; f < 7; ++f) {
    attacks |= (BB_1 << ((tr * 8) + f));
  }
  for (f = tf - 1; f > 0; --f) {
    attacks |= (BB_1 << ((tr * 8) + f));
  }

  return attacks;
}


bb_t precompute_bishop_attacks(index_t square, bb_t blocks)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; ++r, ++f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; --r, ++f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; ++r, --f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; --r, --f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }

  return attacks;
}


bb_t precompute_rook_attacks(index_t square, bb_t blocks)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1; r <= 7; ++r) {
    const bb_t candidate = (BB_1 << ((r * 8) + tf));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1; r >= 0; --r) {
    const bb_t candidate = (BB_1 << ((r * 8) + tf));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (f = tf + 1; f <= 7; ++f) {
    const bb_t candidate = (BB_1 << ((tr * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (f = tf - 1; f >= 0; --f) {
    const bb_t candidate = (BB_1 << ((tr * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }

  return attacks;
}


void initialize_const_data(bb_tables_t* data)
{
  assert(data != nullptr);

  for (index_t i = 0; i < 64; ++i) {
    // Init pawn attacks
    data->pawn_attacks[WHITE][i] = precompute_pawn_attacks(WHITE, i);
    data->pawn_attacks[BLACK][i] = precompute_pawn_attacks(BLACK, i);

    // Init knight attacks
    data->knight_attacks[i] = precompute_knight_attacks(i);

    // Init king attacks
    data->king_attacks[i] = precompute_king_attacks(i);

    // Init masks for bishop and rooks
    data->bishop_masks[i] = precompute_bishop_attack_masks(i);
    data->rook_masks[i] = precompute_rook_attack_masks(i);

    {  // Init bishop and rook attack vector
      const bb_t bishop_attack_mask = data->bishop_masks[i];
      const bb_t rook_attack_mask = data->rook_masks[i];

      const int bishop_num_relevant_bits = bishop_relevant_bits_count[i];
      const int rook_num_relevant_bits = rook_relevant_bits_count[i];

      const int bishop_occupancy_indicies = (1 << bishop_num_relevant_bits);
      const int rook_occupancy_indicies = (1 << rook_num_relevant_bits);

      for (int index = 0; index < bishop_occupancy_indicies; ++index) {
        const bb_t occupancy =
            set_occupancy(index, bishop_num_relevant_bits, bishop_attack_mask);

        const int magic_index = (occupancy * bishop_magic_numbers[i]) >>
                                (64 - bishop_num_relevant_bits);

        data->bishop_attacks[i][magic_index] =
            precompute_bishop_attacks(i, occupancy);
      }

      for (int index = 0; index < rook_occupancy_indicies; ++index) {
        const bb_t occupancy =
            set_occupancy(index, rook_num_relevant_bits, rook_attack_mask);

        const int magic_index = (occupancy * rook_magic_numbers[i]) >>
                                (64 - rook_num_relevant_bits);

        data->rook_attacks[i][magic_index] =
            precompute_rook_attacks(i, occupancy);
      }
    }
  }

  LOG_I << "Bitboard const data initialized" << END_I;
}


piece_t get_piece(const board_t* board, index_t square)
{
  for (int piece = W_PAWN; piece <= B_KING; ++piece) {
    if (GET_BIT(board->bitboards[piece], square)) {
      return static_cast<piece_t>(piece);
    }
  }

  return EMPTY;
}


void cleanup_board(board_t* board)
{
  assert(board != nullptr);
  memset(board->bitboards, 0, sizeof(board->bitboards));

  board->active_color = WHITE;
  board->castling = WQ | WK | BQ | BK;
  board->halfmove_clock = 0;
  board->en_passant = INVALID_INDEX;
  board->fullmove_counter = 1;
  board->zobrist_key = 0;
}
